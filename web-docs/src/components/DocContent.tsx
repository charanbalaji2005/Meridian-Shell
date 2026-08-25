import React, { useEffect, useState } from 'react';
import { DocArticle } from '../data/docsContent';
import { NAV_STRUCTURE } from '../data/docsNav';
import { Copy, Check, ChevronDown, ArrowLeft, ArrowRight } from 'lucide-react';

interface DocContentProps {
  article: DocArticle;
  onNavigate: (id: string) => void;
}

export const DocContent: React.FC<DocContentProps> = ({ article, onNavigate }) => {
  const [copiedLLM, setCopiedLLM] = useState(false);

  // Flatten navigation items to calculate previous & next article
  const allNavItems: { id: string; title: string }[] = [];
  NAV_STRUCTURE.forEach((cat) => {
    cat.items.forEach((item) => {
      allNavItems.push(item);
    });
  });

  const currentIndex = allNavItems.findIndex((item) => item.id === article.id);
  const prevItem = currentIndex > 0 ? allNavItems[currentIndex - 1] : null;
  const nextItem = currentIndex < allNavItems.length - 1 ? allNavItems[currentIndex + 1] : null;

  const handleCopyForLLM = () => {
    const rawContent = `${article.title}\nCategory: ${article.category}\nDate: ${article.lastUpdated}\n\n${article.summary}\n\n` +
      article.body.replace(/<[^>]*>?/gm, '');
    navigator.clipboard.writeText(rawContent.trim());
    setCopiedLLM(true);
    setTimeout(() => setCopiedLLM(false), 2000);
  };

  useEffect(() => {
    // Enhance code blocks with Claude-style header and copy button
    const preBlocks = document.querySelectorAll('.doc-body-content pre');
    preBlocks.forEach((pre) => {
      if (pre.querySelector('.code-header-bar')) return;

      const codeEl = pre.querySelector('code');
      let lang = 'bash';
      if (codeEl) {
        const className = codeEl.className || '';
        const match = className.match(/language-(\w+)/);
        if (match) lang = match[1];
      }

      const headerBar = document.createElement('div');
      headerBar.className = 'code-header-bar';
      headerBar.innerHTML = `
        <span class="code-lang-label">${lang}</span>
        <button class="code-copy-btn" aria-label="Copy code">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
          <span>Copy</span>
        </button>
      `;

      const copyBtn = headerBar.querySelector('.code-copy-btn');
      if (copyBtn) {
        copyBtn.addEventListener('click', () => {
          const text = codeEl?.innerText || pre.textContent || '';
          navigator.clipboard.writeText(text.trim());
          copyBtn.innerHTML = `
            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg>
            <span style="color: #00A8B5; font-weight: 500;">Copied</span>
          `;
          setTimeout(() => {
            copyBtn.innerHTML = `
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
              <span>Copy</span>
            `;
          }, 2000);
        });
      }

      pre.insertBefore(headerBar, pre.firstChild);
    });
  }, [article]);

  return (
    <article className="claude-article">
      <header className="article-title-header">
        <div className="title-row">
          <h1 className="article-serif-title">{article.title}</h1>
          <button onClick={handleCopyForLLM} className="copy-llm-btn" aria-label="Copy for LLM">
            {copiedLLM ? <Check size={14} className="copied-icon" /> : <Copy size={14} />}
            <span>{copiedLLM ? 'Copied for LLM' : 'Copy for LLM'}</span>
            <ChevronDown size={12} className="chevron-icon" />
          </button>
        </div>
        <p className="article-publish-date">{article.lastUpdated}</p>
      </header>

      {article.summary && (
        <p className="article-lead-intro">{article.summary}</p>
      )}

      <div
        className="doc-body-content"
        dangerouslySetInnerHTML={{ __html: article.body }}
      />

      <footer className="article-bottom-pager">
        <div className="pager-container">
          {prevItem ? (
            <button
              onClick={() => onNavigate(prevItem.id)}
              className="claude-pager-card prev-card"
            >
              <ArrowLeft size={16} className="pager-arrow" />
              <div className="pager-meta">
                <span className="pager-label">Previous</span>
                <span className="pager-title">{prevItem.title}</span>
              </div>
            </button>
          ) : <div className="pager-spacer" />}

          {nextItem && (
            <button
              onClick={() => onNavigate(nextItem.id)}
              className="claude-pager-card next-card"
            >
              <div className="pager-meta text-right">
                <span className="pager-label">Next</span>
                <span className="pager-title">{nextItem.title}</span>
              </div>
              <ArrowRight size={16} className="pager-arrow" />
            </button>
          )}
        </div>
      </footer>
    </article>
  );
};
