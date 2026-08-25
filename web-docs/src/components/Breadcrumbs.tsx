import React from 'react';
import { ChevronRight } from 'lucide-react';

interface BreadcrumbsProps {
  category: string;
  title: string;
  onNavigateHome: () => void;
}

export const Breadcrumbs: React.FC<BreadcrumbsProps> = ({
  category,
  title,
  onNavigateHome,
}) => {
  // Format category to Title Case if uppercase
  const formattedCategory = category
    .toLowerCase()
    .split(' ')
    .map((w) => w.charAt(0).toUpperCase() + w.slice(1))
    .join(' ');

  return (
    <nav className="claude-breadcrumbs" aria-label="Breadcrumbs">
      <button onClick={onNavigateHome} className="breadcrumb-link">
        All Collections
      </button>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <button onClick={onNavigateHome} className="breadcrumb-link">
        Meridian Shell
      </button>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <span className="breadcrumb-link">{formattedCategory}</span>
      <ChevronRight size={13} className="breadcrumb-separator" />
      <span className="breadcrumb-current">{title}</span>
    </nav>
  );
};
